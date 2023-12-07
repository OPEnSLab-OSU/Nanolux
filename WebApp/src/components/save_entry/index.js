import style from './style.css';
import {useState} from "preact/hooks";

const Save_Entry = (

    name,
    idx

) => {
    const [value, setValue] = useState(0);

    const valueChanged = (event) => {
        setValue(event.target.value);
    };

    return (
        <div>
            <tr>
                <th>{name}</th>
                <th>{name}</th>
            </tr>
        </div>
    );
}

export default Save_Entry;
